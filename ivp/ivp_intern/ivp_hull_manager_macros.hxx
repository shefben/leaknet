// Copyright (C) Ipion Software GmbH 1999-2000. All rights reserved.

IVP_BOOL IVP_Hull_Manager::are_events_in_hull(){
    if ( sorted_synapses.find_min_value() - hull_value_next_psi < 0.0f){ // no other synapses need to be checked
	return IVP_TRUE;
    }else{
	return IVP_FALSE;
    }
}

void IVP_Hull_Manager::check_hull_synapses(){
    // check all synapses

    // eventually throw hull limit exceeded event
    IVP_FLOAT syn_val;
    int maxcnt = 100;
    // NOTE: guard on has_elements(). find_min_value() safely returns the stored min_value
    // even when the list is empty, but find_min_elem() (below) asserts first_element != UNUSED.
    // An object with no current synapses has nothing to fire, so an empty list must skip the
    // loop (this mirrors IVP_Hull_Manager::delete_hull_manager which guards the same call).
    while ( sorted_synapses.has_elements() &&
	    (syn_val = sorted_synapses.find_min_value() - hull_value_next_psi) < 0.0f){ // no other synapses need to be checked
	// syn hull event is to be fired
	IVP_Listener_Hull *syn = (IVP_Listener_Hull *)sorted_synapses.find_min_elem(); // corresponding synapse
	syn->hull_limit_exceeded_event(this,syn_val); // ATT: syn MUST update/remove itself/its peer from min_hash!
	if (maxcnt-- <0) break;
    }
}

void IVP_Hull_Manager::increase_hull_by_x(IVP_Time t_now, IVP_FLOAT delta_time, IVP_FLOAT gradient_, IVP_FLOAT center_gradient_) {
    // Harden against an inf/NaN gradient coming from an object whose velocity blew up (a
    // degenerate contact, e.g. on a broken world-collision solid). A non-finite gradient
    // would otherwise propagate inf/NaN through every hull value and assert the debug build.
    if ( !(gradient_ <= IVP_U_MINLIST_MAXVALUE && gradient_ >= -IVP_U_MINLIST_MAXVALUE) )
        gradient_ = 0.0f;
    if ( !(center_gradient_ <= IVP_U_MINLIST_MAXVALUE && center_gradient_ >= -IVP_U_MINLIST_MAXVALUE) )
        center_gradient_ = 0.0f;

    IVP_FLOAT dt = t_now - last_vpsi_time;
    hull_value_last_vpsi += gradient * dt;
    hull_center_value_last_vpsi += center_gradient * dt;
    last_vpsi_time = t_now;
    gradient = gradient_ * IVP_HULL_MANAGER_GRADIENT_FACTOR;
    center_gradient = center_gradient_;

    IVP_FLOAT delta = delta_time * gradient;
    hull_value_next_psi = hull_value_last_vpsi + delta;
    // Clamp the result too (hull_value_last_vpsi may already be non-finite from a prior frame).
    if ( !(hull_value_next_psi >= 0.0f) || hull_value_next_psi > IVP_U_MINLIST_MAXVALUE )
    {
        hull_value_last_vpsi = 0.0f;
        hull_value_next_psi = 0.0f;
        gradient = 0.0f;
    }
}

