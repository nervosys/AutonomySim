#!/usr/bin/env python3
"""
AutonomySim RPC Test Server

This script emulates the UE5 AutonomySimRPCServer for testing the Rust client.
It listens on port 41451 and responds to JSON-RPC messages.

Usage:
    python test_rpc_server.py

Then run the Rust example:
    cargo run --example unreal_robotic_swarm --features unreal --release
"""

import json
import socket
import threading
from datetime import datetime
from typing import Dict, Any, Optional

# ANSI colors for output
class Colors:
    HEADER = '\033[95m'
    BLUE = '\033[94m'
    CYAN = '\033[96m'
    GREEN = '\033[92m'
    YELLOW = '\033[93m'
    RED = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'

class AutonomySimTestServer:
    """Test server that emulates the UE5 AutonomySimRPCServer"""
    
    def __init__(self, host: str = "127.0.0.1", port: int = 41451):
        self.host = host
        self.port = port
        self.socket = None
        self.running = False
        self.robots: Dict[int, Dict[str, Any]] = {}
        self.visualization_mode = {
            "show_comm_links": True,
            "show_rf_range": False,
            "show_paths": False,
            "show_labels": False
        }
        self.paused = False
        
    def start(self):
        """Start the test server"""
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.socket.bind((self.host, self.port))
        self.socket.listen(1)
        self.running = True
        
        print(f"{Colors.HEADER}╔══════════════════════════════════════════════════════════════╗{Colors.ENDC}")
        print(f"{Colors.HEADER}║      AUTONOMYSIM RPC TEST SERVER                             ║{Colors.ENDC}")
        print(f"{Colors.HEADER}╚══════════════════════════════════════════════════════════════╝{Colors.ENDC}")
        print()
        print(f"{Colors.GREEN}✓ Server listening on {self.host}:{self.port}{Colors.ENDC}")
        print(f"{Colors.CYAN}  Waiting for Rust client connection...{Colors.ENDC}")
        print()
        
        while self.running:
            try:
                client_socket, address = self.socket.accept()
                print(f"{Colors.GREEN}✓ Client connected from {address}{Colors.ENDC}")
                self._handle_client(client_socket)
            except Exception as e:
                if self.running:
                    print(f"{Colors.RED}Error accepting connection: {e}{Colors.ENDC}")
                    
    def _handle_client(self, client_socket: socket.socket):
        """Handle a connected client"""
        buffer = ""
        
        try:
            while self.running:
                data = client_socket.recv(65536)
                if not data:
                    break
                    
                buffer += data.decode('utf-8')
                
                # Process complete messages (newline-delimited)
                while '\n' in buffer:
                    line, buffer = buffer.split('\n', 1)
                    if line.strip():
                        response = self._process_message(line)
                        client_socket.sendall((response + '\n').encode('utf-8'))
                        
        except Exception as e:
            print(f"{Colors.RED}Error handling client: {e}{Colors.ENDC}")
        finally:
            client_socket.close()
            print(f"{Colors.YELLOW}Client disconnected{Colors.ENDC}")
            
    def _process_message(self, message: str) -> str:
        """Process a JSON-RPC message and return response"""
        try:
            request = json.loads(message)
            method = request.get('method', 'unknown')
            params = request.get('params', {})
            request_id = request.get('id', 0)
            
            # Route to handler
            handler = getattr(self, f'_handle_{method.replace("-", "_")}', None)
            if handler:
                result = handler(params)
                return self._success_response(request_id, result)
            else:
                print(f"{Colors.YELLOW}  Unknown method: {method}{Colors.ENDC}")
                return self._error_response(request_id, -1, f"Unknown method: {method}")
                
        except json.JSONDecodeError as e:
            return self._error_response(0, -1, f"Invalid JSON: {e}")
            
    def _success_response(self, request_id: int, result: Any = None) -> str:
        return json.dumps({
            "jsonrpc": "2.0",
            "id": request_id,
            "result": result
        })
        
    def _error_response(self, request_id: int, code: int, message: str) -> str:
        return json.dumps({
            "jsonrpc": "2.0",
            "id": request_id,
            "error": {"code": code, "message": message}
        })
        
    # === RPC Handlers ===
    
    def _handle_spawn_robots(self, params: dict) -> dict:
        """Handle spawn_robots RPC"""
        robots = params.get('robots', [])
        
        # Count by type
        type_counts = {}
        for robot in robots:
            robot_id = robot.get('id', 0)
            robot_type = robot.get('robot_type', 'Unknown')
            self.robots[robot_id] = robot
            type_counts[robot_type] = type_counts.get(robot_type, 0) + 1
            
        print(f"{Colors.GREEN}  ✓ spawn_robots: {len(robots)} robots{Colors.ENDC}")
        for rtype, count in type_counts.items():
            color = {
                'Scout': Colors.RED,
                'Transport': Colors.BLUE,
                'Combat': Colors.GREEN,
                'Relay': Colors.YELLOW,
                'Coordinator': '\033[95m'  # Purple
            }.get(rtype, Colors.ENDC)
            print(f"    {color}• {rtype}: {count}{Colors.ENDC}")
            
        return {"spawned": len(robots)}
        
    def _handle_update_positions(self, params: dict) -> dict:
        """Handle update_positions RPC"""
        positions = params.get('positions', [])
        
        for pos in positions:
            robot_id = pos.get('id', 0)
            if robot_id in self.robots:
                self.robots[robot_id].update({
                    'x': pos.get('x', 0),
                    'y': pos.get('y', 0),
                    'z': pos.get('z', 0),
                    'yaw': pos.get('yaw', 0)
                })
                
        # Only print every 100 updates
        if len(positions) > 0:
            sample = positions[0]
            print(f"{Colors.CYAN}  → update_positions: {len(positions)} robots, "
                  f"sample[0]: ({sample.get('x', 0):.1f}, {sample.get('y', 0):.1f}, {sample.get('z', 0):.1f}){Colors.ENDC}")
            
        return {"updated": len(positions)}
        
    def _handle_update_telemetry(self, params: dict) -> dict:
        """Handle update_telemetry RPC"""
        telemetry = params.get('telemetry', [])
        
        jammed_count = sum(1 for t in telemetry if t.get('is_jammed', False))
        low_battery = sum(1 for t in telemetry if t.get('battery_percent', 100) < 20)
        
        print(f"{Colors.YELLOW}  → update_telemetry: {len(telemetry)} robots, "
              f"{jammed_count} jammed, {low_battery} low battery{Colors.ENDC}")
            
        return {"updated": len(telemetry)}
        
    def _handle_simulation_step(self, params: dict) -> dict:
        """Handle simulation_step RPC"""
        dt = params.get('delta_time', 0.02)
        return {"stepped": True, "dt": dt}
        
    def _handle_clear_all_robots(self, params: dict) -> dict:
        """Handle clear_all_robots RPC"""
        count = len(self.robots)
        self.robots.clear()
        print(f"{Colors.RED}  ✓ clear_all_robots: cleared {count} robots{Colors.ENDC}")
        return {"cleared": count}
        
    def _handle_draw_debug_lines(self, params: dict) -> dict:
        """Handle draw_debug_lines RPC"""
        lines = params.get('lines', [])
        print(f"{Colors.CYAN}  → draw_debug_lines: {len(lines)} lines{Colors.ENDC}")
        return {"drawn": len(lines)}
        
    def _handle_draw_debug_spheres(self, params: dict) -> dict:
        """Handle draw_debug_spheres RPC"""
        spheres = params.get('spheres', [])
        print(f"{Colors.CYAN}  → draw_debug_spheres: {len(spheres)} spheres{Colors.ENDC}")
        return {"drawn": len(spheres)}
        
    def _handle_clear_debug(self, params: dict) -> dict:
        """Handle clear_debug RPC"""
        print(f"{Colors.CYAN}  ✓ clear_debug{Colors.ENDC}")
        return {"cleared": True}
        
    def _handle_set_visualization_mode(self, params: dict) -> dict:
        """Handle set_visualization_mode RPC"""
        self.visualization_mode.update(params)
        print(f"{Colors.CYAN}  ✓ set_visualization_mode: {params}{Colors.ENDC}")
        return self.visualization_mode
        
    def _handle_pause(self, params: dict) -> dict:
        """Handle pause RPC"""
        self.paused = True
        print(f"{Colors.YELLOW}  ⏸ PAUSED{Colors.ENDC}")
        return {"paused": True}
        
    def _handle_resume(self, params: dict) -> dict:
        """Handle resume RPC"""
        self.paused = False
        print(f"{Colors.GREEN}  ▶ RESUMED{Colors.ENDC}")
        return {"paused": False}
        
    def _handle_reset(self, params: dict) -> dict:
        """Handle reset RPC"""
        self.robots.clear()
        self.paused = False
        print(f"{Colors.RED}  ✓ RESET{Colors.ENDC}")
        return {"reset": True}
        
    def _handle_get_all_states(self, params: dict) -> dict:
        """Handle get_all_states RPC"""
        return {"robots": list(self.robots.values())}
        
    def stop(self):
        """Stop the server"""
        self.running = False
        if self.socket:
            self.socket.close()


def main():
    server = AutonomySimTestServer()
    try:
        server.start()
    except KeyboardInterrupt:
        print(f"\n{Colors.YELLOW}Shutting down...{Colors.ENDC}")
        server.stop()


if __name__ == "__main__":
    main()
