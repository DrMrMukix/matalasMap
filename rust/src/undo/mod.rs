use crate::core::types::{EditorMode, Rect};
use crate::political::country::CountryId;
use crate::political::grid::PoliticalGrid;
use crate::terrain::grid::TerrainGrid;
use std::collections::VecDeque;

pub enum UndoStep {
    Terrain {
        rect: Rect,
        data: Vec<u8>,
    },
    Political {
        rect: Rect,
        data: Vec<CountryId>,
    },
}

pub struct UndoManager {
    undo_stack: VecDeque<UndoStep>,
    redo_stack: VecDeque<UndoStep>,
    max_history: usize,
}

impl UndoManager {
    pub fn new(max_history: usize) -> Self {
        Self {
            undo_stack: VecDeque::with_capacity(max_history),
            redo_stack: VecDeque::with_capacity(max_history),
            max_history,
        }
    }

    pub fn record_terrain_before(&mut self, rect: Rect, terrain: &TerrainGrid) {
        let data = terrain.extract_sub_rect(&rect);
        if self.undo_stack.len() >= self.max_history {
            self.undo_stack.pop_front();
        }
        self.undo_stack.push_back(UndoStep::Terrain { rect, data });
        self.redo_stack.clear();
    }

    pub fn record_political_before(&mut self, rect: Rect, political: &PoliticalGrid) {
        let data = political.extract_sub_rect(&rect);
        if self.undo_stack.len() >= self.max_history {
            self.undo_stack.pop_front();
        }
        self.undo_stack.push_back(UndoStep::Political { rect, data });
        self.redo_stack.clear();
    }

    pub fn can_undo(&self) -> bool {
        !self.undo_stack.is_empty()
    }

    pub fn can_redo(&self) -> bool {
        !self.redo_stack.is_empty()
    }

    pub fn undo(
        &mut self,
        terrain: &mut TerrainGrid,
        political: &mut PoliticalGrid,
    ) -> Option<(Rect, EditorMode)> {
        let step = self.undo_stack.pop_back()?;

        match step {
            UndoStep::Terrain { rect, data: old_data } => {
                let current_data = terrain.extract_sub_rect(&rect);
                self.redo_stack.push_back(UndoStep::Terrain {
                    rect,
                    data: current_data,
                });
                terrain.restore_sub_rect(&rect, &old_data);
                Some((rect, EditorMode::Terrain))
            }
            UndoStep::Political { rect, data: old_data } => {
                let current_data = political.extract_sub_rect(&rect);
                self.redo_stack.push_back(UndoStep::Political {
                    rect,
                    data: current_data,
                });
                political.restore_sub_rect(&rect, &old_data);
                Some((rect, EditorMode::Political))
            }
        }
    }

    pub fn redo(
        &mut self,
        terrain: &mut TerrainGrid,
        political: &mut PoliticalGrid,
    ) -> Option<(Rect, EditorMode)> {
        let step = self.redo_stack.pop_back()?;

        match step {
            UndoStep::Terrain { rect, data: new_data } => {
                let current_data = terrain.extract_sub_rect(&rect);
                self.undo_stack.push_back(UndoStep::Terrain {
                    rect,
                    data: current_data,
                });
                terrain.restore_sub_rect(&rect, &new_data);
                Some((rect, EditorMode::Terrain))
            }
            UndoStep::Political { rect, data: new_data } => {
                let current_data = political.extract_sub_rect(&rect);
                self.undo_stack.push_back(UndoStep::Political {
                    rect,
                    data: current_data,
                });
                political.restore_sub_rect(&rect, &new_data);
                Some((rect, EditorMode::Political))
            }
        }
    }

    pub fn clear(&mut self) {
        self.undo_stack.clear();
        self.redo_stack.clear();
    }
}
