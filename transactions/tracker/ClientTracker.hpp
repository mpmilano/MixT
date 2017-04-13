#pragma once
#include "Tracker.hpp"

namespace myria{
  namespace tracker{

    template<typename label> struct TombHolder;
    
    template<typename label>
    struct TombHolder<Label<label> >{
      std::unique_ptr<std::vector<tracker::Tombstone> >
      obligations{new std::vector<tracker::Tombstone>()};
    protected:
      ~TombHolder() = default;
    };
    
    template<typename... labels>
    struct ClientTracker : public TombHolder<labels>... {
      Tracker local_tracker;
      
      template<typename phase> std::vector<Tombstone>& tombstones_for_phase(){
	return *TombHolder<typename phase::label>::obligations;
      }
      template<typename phase> void clear_tombstones_for_phase(){
	TombHolder<typename phase::label>::obligations->clear();
      }
      template<typename phase>
	void set_phase_after(std::unique_ptr<std::vector<tracker::Tombstone> > ptr){
	TombHolder<mutils::follows_in_sequence<typename phase::label, labels...> >::
	  obligations = std::move(ptr);
      }
    };

  }}
