def spectre():
    N = 256 #nb de plages du spectre
    t,temp,val_en=[],[],0
    spectre_photon_hautgain = [0 for i in range(N)]
    spectre_photon_basgain = [0 for i in range(N)]
    spectre_electron_bloc1_basgain= [0 for i in range(N)]
    spectre_electron_bloc1_hautgain = [0]*N
    spectre_electron_bloc2_basgain = [0 for i in range(N)]
    spectre_electron_bloc2_hautgain = [0 for i in range(N)]
    spectre_electron_bloc3_hautgain = [0 for i in range(N)]
    spectre_electron_bloc3_basgain = [0 for i in range(N)]
    spectre_electron_bloc4_hautgain = [0 for i in range(N)]
    spectre_electron_bloc4_basgain = [0 for i in range(N)]
    spectre_electron_bloc5_hautgain = [0 for i in range(N)]
    spectre_electron_bloc5_basgain = [0 for i in range(N)]
 
    energie_max_photon = (2**15)*4 #un photon allume 4 capteurs au max chacun code sur 14 bits
    pas_sp_photon = energie_max_photon//N
    energie_max_bloc1_electron = (2**15)*4 #le bloc 1 a 4 capteurs
    energie_max_bloc234_electron = (2**15)*3 #les autres blocs ont 3 capteurs
    pas_sp_electron_bloc1 = energie_max_bloc1_electron//N
    pas_sp_electron_bloc234 = energie_max_bloc234_electron//N
    f =open('splitLast','r')
    for line in f:
            t = line.split()
            for k in range(1,17):
                if t[2*k-1] !='-1' and t[2*k-1] != '2':
                    temp.append(k) #on releve les no des capteurs actives et non satures
            if temp!=[]:
                if max(temp)<5: #il s agit d un photon
                    for k in temp:
                            val_en = val_en + int(t[2*k],16) #on releve l energie du photon (gain a determiner ensuite)
                    if t[1]=='0' or t[3]=='0' or t[5]=='0' or t[7]=='0': #haut gain
                        for i in range(N):
                            if i*pas_sp_photon < val_en < (i+1)*pas_sp_photon:
                                spectre_photon_hautgain[i] += 1 #on la place dans le spectre
                    if t[1]=='1' or t[3]=='1' or t[5]=='1' or t[7]=='1': #bas gain
                        for i in range(N):
                            if i*pas_sp_photon < val_en < (i+1)*pas_sp_photon:
                                spectre_photon_basgain[i] += 1 #idem
                if max(temp)>4: # c est un electron
                    if 1 or 2 or 3 or 4 in temp: #bloc 1 active
                        for i in temp:#on releve l energie des capteurs 1 a 4
                            if 0<i<5:
                                val_en = val_en + int(t[2*i],16)
                        if t[1]=='0' or t[3]=='0' or t[5]=='0' or t[7]=='0': #haut gain
                            for i in range(N):
                                if i*pas_sp_electron_bloc1 < val_en < (i+1)*pas_sp_electron_bloc1:
                                    spectre_electron_bloc1_hautgain[i] += 1
                        if t[1]=='1' or t[3]=='1' or t[5]=='1' or t[7]=='1': #bas gain
                            for i in range(N):
                                if i*pas_sp_electron_bloc1 < val_en < (i+1)*pas_sp_electron_bloc1:
                                    spectre_electron_bloc1_basgain[i] += 1
                    val_en = 0
                    if 5 or 6 or 7 in temp: #bloc 2 active
                        for i in temp:#on releve l energie des capteurs 5 a 7
                            if 4<i<8:
                                val_en = val_en + int(t[2*i],16)
                        if t[9]=='0' or t[11]=='0' or t[13]=='0': #haut gain
                                for i in range(N):
                                    if i*pas_sp_electron_bloc234 < val_en < (i+1)*pas_sp_electron_bloc234:
                                        spectre_electron_bloc2_hautgain[i] += 1
                        if t[9]=='1' or t[11]=='1' or t[13]=='1': #bas gain
                                for i in range(N):
                                    if i*pas_sp_electron_bloc234 < val_en < (i+1)*pas_sp_electron_bloc234:
                                        spectre_electron_bloc2_basgain[i] += 1
                    val_en = 0
                    if 8 or 9 or 10 in temp: #bloc 3 active
                        for i in temp:#on releve l energie des capteurs 8 a 10
                            if 7<i<11:
                                val_en = val_en + int(t[2*i],16)
                        if t[15]=='0' or t[17]=='0' or t[19]=='0': #haut gain
                            for i in range(N):
                                if i*pas_sp_electron_bloc234 < val_en < (i+1)*pas_sp_electron_bloc234:
                                    spectre_electron_bloc3_hautgain[i] += 1
                        if t[15]=='1' or t[17]=='1' or t[19]=='1': #bas gain
                            for i in range(N):
                                if i*pas_sp_electron_bloc234 < val_en < (i+1)*pas_sp_electron_bloc234:
                                    spectre_electron_bloc3_basgain[i] += 1
                    val_en = 0
                    if 11 or 12 or 13 in temp: #bloc 4 active
                        for i in temp:#on releve l energie des capteurs 11 a 13
                            if 10<i<14:
                                val_en = val_en + int(t[2*i],16)
                        if t[21]=='0' or t[23]=='0' or t[25]=='0': #haut gain
                            for i in range(N):
                                if i*pas_sp_electron_bloc234 < val_en < (i+1)*pas_sp_electron_bloc234:
                                    spectre_electron_bloc4_hautgain[i] += 1
                        if t[21]=='1' or t[23]=='1' or t[25]=='1': #bas gain
                            for i in range(N):
                                if i*pas_sp_electron_bloc234 < val_en < (i+1)*pas_sp_electron_bloc234:
                                    spectre_electron_bloc4_basgain[i] += 1
                    val_en = 0
                    if 14 or 15 or 16 in temp: #bloc 5 active
                        for i in temp:#on releve l energie des capteurs 14 a 16
                            if 13<i<17:
                                val_en = val_en + int(t[2*i],16)
                        if t[27]=='0' or t[29]=='0' or t[31]=='0': #haut gain
                            for i in range(N):
                                if i*pas_sp_electron_bloc234 < val_en < (i+1)*pas_sp_electron_bloc234:
                                    spectre_electron_bloc5_hautgain[i] += 1
                        if t[27]=='1' or t[29]=='1' or t[31]=='1': #haut gain
                            for i in range(N):
                                if i*pas_sp_electron_bloc234 < val_en < (i+1)*pas_sp_electron_bloc234:
                                    spectre_electron_bloc5_basgain[i] += 1
                        
                            
            temp,val_en=[],0
    f.close()

    while spectre_electron_bloc1_hautgain[-1]==0:
        spectre_electron_bloc1_hautgain[-1:]=[] #on retire les 0 de fin de spectre
    while spectre_electron_bloc1_basgain[-1]==0:
        spectre_electron_bloc1_basgain[-1:]=[]

    while spectre_electron_bloc2_hautgain[-1]==0:
        spectre_electron_bloc2_hautgain[-1:]=[] #on retire les 0 de fin de spectre
    while spectre_electron_bloc2_basgain[-1]==0:
        spectre_electron_bloc2_basgain[-1:]=[]

    while spectre_electron_bloc3_hautgain[-1]==0:
        spectre_electron_bloc3_hautgain[-1:]=[] #on retire les 0 de fin de spectre
    while spectre_electron_bloc3_basgain[-1]==0:
        spectre_electron_bloc3_basgain[-1:]=[]

    while spectre_electron_bloc4_hautgain[-1]==0:
        spectre_electron_bloc4_hautgain[-1:]=[] #on retire les 0 de fin de spectre
    while spectre_electron_bloc4_basgain[-1]==0:
        spectre_electron_bloc4_basgain[-1:]=[]

    while spectre_electron_bloc5_hautgain[-1]==0:
        spectre_electron_bloc5_hautgain[-1:]=[] #on retire les 0 de fin de spectre
    while spectre_electron_bloc5_basgain[-1]==0:
        spectre_electron_bloc5_basgain[-1:]=[]

    while spectre_photon_hautgain[-1]==0:
        spectre_photon_hautgain[-1:]=[] #on retire les 0 de fin de spectre
    while spectre_photon_basgain[-1]==0:
        spectre_photon_basgain[-1:]=[]

    
    g =open('Spectre_photon_electron.txt','w')
    g.write('Sp_el_b1_hg:'+'\n')
    for i in spectre_electron_bloc1_hautgain:
        g.write(str(i)+' ')
    g.write('\n')
    g.write('Sp_el_b1_bg:'+'\n')
    for i in spectre_electron_bloc1_basgain:
        g.write(str(i)+' ')
    g.write('\n')
    g.write('Sp_el_b2_hg:'+'\n')
    for i in spectre_electron_bloc2_hautgain:
        g.write(str(i)+' ')
    g.write('\n')
    g.write('Sp_el_b2_bg:'+'\n')
    for i in spectre_electron_bloc2_basgain:
        g.write(str(i)+' ')
    g.write('\n')
    g.write('Sp_el_b3_hg:'+'\n')
    for i in spectre_electron_bloc3_hautgain:
        g.write(str(i)+' ')
    g.write('\n')
    g.write('Sp_el_b3_bg:'+'\n')
    for i in spectre_electron_bloc3_basgain:
        g.write(str(i)+' ')
    g.write('\n')
    g.write('Sp_el_b4_hg:'+'\n')
    for i in spectre_electron_bloc4_hautgain:
        g.write(str(i)+' ')
    g.write('\n')
    g.write('Sp_el_b4_bg:'+'\n')
    for i in spectre_electron_bloc4_basgain:
        g.write(str(i)+' ')
    g.write('\n')
    g.write('Spectre electron bloc haut gain 5:'+'\n')
    for i in spectre_electron_bloc5_hautgain:
        g.write(str(i)+' ')
    g.write('\n')
    g.write('Sp_el_b5_bg:'+'\n')
    for i in spectre_electron_bloc5_basgain:
        g.write(str(i)+' ')
    g.write('\n')
    g.write('Sp_ph_bg:'+'\n')
    for i in spectre_photon_basgain:
        g.write(str(i)+' ')
    g.write('\n')
    g.write('Sp_ph_hg:'+'\n')
    for i in spectre_photon_hautgain:
        g.write(str(i)+' ')
    g.close()
    return

spectre()
# hypothese : dans un meme bloc on ne peut pas avoir deux gains differents (A VERIFIER)
#
