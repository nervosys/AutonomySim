#ifndef autonomylib_vehicles_simpleflight_firmware_IUpdatable_hpp
#define autonomylib_vehicles_simpleflight_firmware_IUpdatable_hpp

namespace simple_flight {

class IUpdatable {
  public:
    virtual void reset() {
        if (reset_called && !update_called)
            throw std::runtime_error("Multiple reset() calls detected without call to update()");

        reset_called = true;
    }
    virtual void update() {
        if (!reset_called)
            throw std::runtime_error("reset() must be called first before update()");
        update_called = true;
    }

    virtual ~IUpdatable() = default;

  protected:
    void clearResetUpdateAsserts() {
        reset_called = false;
        update_called = false;
    }

  private:
    bool reset_called = false;
    bool update_called = false;
};

} // namespace simple_flight

#endif
