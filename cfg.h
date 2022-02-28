#define MODULE_TAG "mpp_dec_cfg"

#include "rk_vdec_cfg.h"

#include "mpp_env.h"
#include "mpp_log.h"
#include "mpp_mem.h"
#include "mpp_time.h"
#include "mpp_common.h"
#include "mpp_thread.h"
#include "rk_type.h"

#include "mpp_dec_cfg_impl.h"
class MppDecCfgService
{
private:
    MppDecCfgService();
    ~MppDecCfgService();
    MppDecCfgService(const MppDecCfgService &);
    MppDecCfgService &operator=(const MppDecCfgService &);

    MppTrie mCfgApi;

public:
    static MppDecCfgService *get() {
        static Mutex lock;
        static MppDecCfgService instance;

        AutoMutex auto_lock(&lock);
        return &instance;
    }

    MppTrie get_api() { return mCfgApi; };
};

MPP_RET mpp_dec_cfg_init(MppDecCfg *cfg);