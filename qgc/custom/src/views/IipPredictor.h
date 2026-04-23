#pragma once

namespace m130::views {

/// Instantaneous Impact Point prediction for a ballistic body.
///
/// First-order prediction — assumes zero drag, flat Earth, and constant
/// gravitational acceleration. Used by the Range Safety console as an
/// always-visible "where would it land if all thrust and control were lost
/// right now?" cue (REQ-M130-GCS-SAFE-007 / SAFE-008).
///
/// Frames:
///   - Downrange (x), crossrange (y) are horizontal, metres.
///   - Up (z) is vertical, metres above ground (AGL).
///   - Velocities in m/s in the same frame.
///   - Gravity defaults to 9.80665 m/s² positive downward.
///
/// Output time_to_impact is NaN when no impact exists (body rising forever —
/// mathematically this requires negative gravity which the model rejects).
struct IipResult {
    double impact_downrange_m  = 0.0;
    double impact_crossrange_m = 0.0;
    double time_to_impact_s    = 0.0;
    bool   valid = false;
};

class IipPredictor
{
public:
    static constexpr double kGravity = 9.80665;

    /// Pure prediction. @p g must be positive.
    static IipResult predict(double pos_down_m, double pos_cross_m, double pos_up_m,
                             double vel_down_m_s, double vel_cross_m_s, double vel_up_m_s,
                             double g = kGravity) noexcept;
};

} // namespace m130::views
