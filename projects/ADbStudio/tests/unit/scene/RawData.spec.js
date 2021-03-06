import { mount  } from '@vue/test-utils';
import Vuex from 'vuex';
import RawData from "@/components/scene/RawData";
import scene_store from '../../../src/store/modules/scene'

import { localVue, i18n,scene_state } from '../TestUtils'


describe('RawData',() => {
    let wrapper;
    let store;

    beforeAll(() => {
        
        localVue.use(Vuex)
        store = new Vuex.Store({
            modules: {
                scene: {
                    namespaced: true,
                    state: scene_state,
                    getters: scene_store.getters
                }
              }
        });
        
        wrapper = mount(RawData, {
            localVue,
            i18n,
            store,
        });
    })
    it('renders all elements', () => {
        expect(wrapper.vm.data.nodes.length).toBe(2);
        expect(wrapper.vm.data.edges.length).toBe(1);
        expect(wrapper.vm.data.nodes[0].id).toBe(1);
        expect(wrapper.vm.data.nodes[1].id).toBe(2);
        expect(wrapper.vm.data.edges[0].id).toBe(-1);
    })
})
