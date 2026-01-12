#include "base/type.hpp"
#include "gfx/camera.hpp"
#include "gfx/grid.hpp"
#include "scenario/type.hpp"
#include "sim/barnes_hut_morton.hpp"
#include "sim/type.hpp"

namespace nbody::scenario {
using namespace nbody::base::type;

class BHM40K : public ScenarioInterface {
   public:
    using Float = F64;
    using Body  = sim::BodyT<F64>;

    BHM40K()  = default;
    ~BHM40K() = default;

    void init(const gfx::Window& window) override;
    void step(const gfx::Window& window) override;

   private:
    void controls();
    void ui();

    sim::BarnesHutMorton<F64> m_sim_bhm{};
    gfx::Camera<Float>        m_camera{};
    gfx::Grid                 m_ui_grid{};

    bool  m_is_paused     = false;
    bool  m_is_ui_visible = true;
    Float m_time_factor   = 1.0;
    Float m_scale_factor  = 1.0;
};
}  // namespace nbody::scenario
