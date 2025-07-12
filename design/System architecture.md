## Video channel state machine

- State machine checks for allowed transitions only
- At `VideoChannel` level, checks are required that transition events succeed before transitioning state
    - For example, it is the responsibility of the `VideoChannel` to confirm that a video is loaded before transitioning from `UNLOADED` to `STOPPED`