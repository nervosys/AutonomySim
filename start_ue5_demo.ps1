#!/usr/bin/env pwsh
# Quick start script for AutonomySim UE5 demo

$ErrorActionPreference = "Stop"

Write-Host "🚀 Starting AutonomySim UE5 Demo" -ForegroundColor Cyan
Write-Host ""

# Step 1: Start UE5
Write-Host "[1/3] Opening Unreal Engine 5..." -ForegroundColor Yellow
Start-Process "C:\Users\adamm\UE5Projects\AutonomySimDemo\AutonomySimDemo.uproject"

Write-Host ""
Write-Host "Waiting for Unreal Engine to load (30 seconds)..." -ForegroundColor Yellow
Start-Sleep -Seconds 30

# Step 2: Instructions
Write-Host ""
Write-Host "[2/3] Setup in Unreal Engine:" -ForegroundColor Yellow
Write-Host "   1. Click 'Play' button in toolbar" -ForegroundColor Cyan
Write-Host "   2. Check Output Log for '✅ AutonomySim RPC Server started on port 41451'" -ForegroundColor Cyan
Write-Host ""
Write-Host "Press Enter when you've clicked 'Play' in UE5..." -ForegroundColor Green
Read-Host

# Step 3: Run Rust simulation
Write-Host ""
Write-Host "[3/3] Starting Rust simulation..." -ForegroundColor Yellow
Write-Host ""

Set-Location "C:\Users\adamm\dev\nervosys\modeling\simulation\AutonomySim"
cargo run --example unreal_robotic_swarm --features unreal --release

Write-Host ""
Write-Host "✅ Demo complete!" -ForegroundColor Green
