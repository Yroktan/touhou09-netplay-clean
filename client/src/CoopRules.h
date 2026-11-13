#pragma once


namespace coop {
struct Rules {
    enum Phase { Waves, Boss, Victory, Defeat };
    Phase phase;
    unsigned tick, phaseTick, wave;
    int lives[2], help, bossHealth, bossMaxHealth;
    unsigned supportCooldown[2];
    unsigned teamBonus;
    Rules() { Reset(); }
    void Reset() {
        phase=Waves; tick=phaseTick=wave=0;
        lives[0]=lives[1]=3; help=3;
        bossMaxHealth=bossHealth=1500;
        supportCooldown[0]=supportCooldown[1]=0; teamBonus=0;
    }
    bool Playing() const { return phase==Waves || phase==Boss; }
    bool Alive(int side) const { return side>=0 && side<2 && lives[side]>0; }
    bool Revive(int side) {
        if(!Playing() || Alive(side) || help<=0) return false;
        --help; lives[side]=1; return true;
    }
    void Hit(int side) {
        if(!Playing() || !Alive(side)) return;
        --lives[side];
        if(!Alive(0) && !Alive(1)) {

            if(!Revive(0)) phase=Defeat;
        }
    }
    bool Support(int side, int level) {
        if(!Playing() || !Alive(side) || level<1 || level>3 || supportCooldown[side]) return false;
        int cost=level==3 ? 2 : 1;
        if(help<cost) return false;
        help-=cost; supportCooldown[side]=180;
        if(!Alive(1-side)) lives[1-side]=1;
        if(level==3 && phase==Boss) DamageBoss(300);
        return true;
    }
    void DamageBoss(int amount) {
        if(phase!=Boss || amount<=0) return;
        bossHealth-=amount;
        if(bossHealth<=0) { bossHealth=0; phase=Victory; teamBonus+=50000; }
    }
    bool Step() {
        if(!Playing()) return false;
        ++tick; ++phaseTick;
        for(int side=0;side<2;++side) if(supportCooldown[side]) --supportCooldown[side];
        if(phase==Waves && phaseTick>=1200) {
            phaseTick=0; ++wave; teamBonus+=10000;
            if(help<5) ++help;
            Revive(0); Revive(1);
            if(wave==3) phase=Boss;
            return true;
        }
        return false;
    }
};
}
