case 31: { //grxlogsethandlerstatuscmd
                static uint16_t msgid = 0;
                static uint8_t fnccode=0;
                ImGui::InputScalar("msgid",ImGuiDataType_U16, &msgid);
                ImGui::InputScalar("fnccode",ImGuiDataType_U8, &fnccode);

                ImGui::InputScalar("id", ImGuiDataType_U16, &command->grxlogsethandlerstatuscmd.param.id);
                ImGui::InputScalar("option", ImGuiDataType_S32, &command->grxlogsethandlerstatuscmd.param.options);
                ImGui::InputText("libpath", command->grxlogsethandlerstatuscmd.param.libpath, sizeof(command->grxlogsethandlerstatuscmd.param.libpath));
                ImGui::InputText("funcname", command->grxlogsethandlerstatuscmd.param.funcName, sizeof(command->grxlogsethandlerstatuscmd.param.funcName));
                
                if(ImGui::Button("Generate CMD")) {
                    msgid = htons(msgid);
                    uint8_t stream[2] ={0,};
                    uint8_t sequence[2] = {0xC0,0x00};
                    uint8_t length[2] = {0x00, sizeof(GRX_LogSetHandlerStatusCmd_t)};
                    memcpy(command->grxlogsethandlerstatuscmd.CmdHeader, &msgid, sizeof(uint16_t));
                    memcpy(command->grxlogsethandlerstatuscmd.CmdHeader + 2, sequence, sizeof(uint16_t));
                    memcpy(command->grxlogsethandlerstatuscmd.CmdHeader + 4, length, sizeof(uint16_t));
                    memcpy(command->grxlogsethandlerstatuscmd.CmdHeader + 7, &fnccode, sizeof(uint8_t));
                    pthread_join(p_thread[4], NULL);
                    packetsign * TestPacket = (packetsign *)malloc(2+2+4+sizeof(GRX_LogSetHandlerStatusCmd_t)); // 8 is for data
                    TestPacket->Identifier = HVD_TEST;
                    TestPacket->PacketType = MIM_PT_TMTC_TEST;
                    TestPacket->Length = sizeof(GRX_LogSetHandlerStatusCmd_t);
                    
                    uint16_t len = sizeof(GRX_LogSetHandlerStatusCmd_t);
                    const uint8_t *byteptr = (uint8_t *)&command->grxlogsethandlerstatuscmd;
                    uint8_t checksum = 0xFF;
                    while (len--) {
                        checksum ^= *(byteptr++);
                    }
                    memcpy(command->grxlogsethandlerstatuscmd.CmdHeader+6, &checksum, sizeof(uint8_t));
                    memcpy(TestPacket->Data, &command->grxlogsethandlerstatuscmd, sizeof(GRX_LogSetHandlerStatusCmd_t));
                    

                    pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)TestPacket);
                }
                //ImGui::Text("0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x || %u %u %u %u %u %u %u",command->grxlogsethandlerstatuscmd.CmdHeader[0],
                //                command->grxlogsethandlerstatuscmd.CmdHeader[1], command->grxlogsethandlerstatuscmd.CmdHeader[2], command->grxlogsethandlerstatuscmd.CmdHeader[3], command->grxlogsethandlerstatuscmd.CmdHeader[4], command->grxlogsethandlerstatuscmd.CmdHeader[5], command->grxlogsethandlerstatuscmd.CmdHeader[6], command->grxlogsethandlerstatuscmd.CmdHeader[7],
                //                command->grxlogsethandlerstatuscmd.Timeout, command->grxlogsethandlerstatuscmd.ItemSize, command->grxlogsethandlerstatuscmd.ItemCount, command->grxlogsethandlerstatuscmd.Node, command->grxlogsethandlerstatuscmd.TableId, command->grxlogsethandlerstatuscmd.Address, command->grxlogsethandlerstatuscmd.Type);
                break;
            }