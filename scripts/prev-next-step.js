main: {
    const self = document.currentScript;

    const path_next = self.getAttribute("path_next");
    const path_prev = self.getAttribute("path_prev");
    const do_prev = path_prev == null ? false : true;
    const do_next = path_next == null ? false : true;
    if (!do_next && !do_prev) break main;

    var label_next = self.getAttribute("label_next");
    if (label_next == null) {
        if (path_next == "/") label_next = "Home";
        else label_next = "Next step";
    }
    var label_prev = self.getAttribute("label_prev");
    if (label_prev == null) {
        if (path_prev == "/") label_prev = "Home";
        else label_prev = "Previous step"
    }

    const mydiv = document.createElement("div");
    mydiv.setAttribute ("width", "100%");

    if (do_prev) {
        const left = document.createElement ("a");
        left.setAttribute ("class", "prev-step");
        left.setAttribute ("href", path_prev);
        left.textContent = label_prev;
        left.style.width = do_next ? "50%" : "100%";
        mydiv.appendChild (left);
    }

    if (do_next) {
        const right = document.createElement ("a");
        right.setAttribute ("class", "next-step");
        right.setAttribute ("href", path_next);
        right.textContent = label_next;
        right.style.width = do_prev ? "50%" : "100%";
        mydiv.appendChild (right);
    }
    self.parentNode.insertBefore(mydiv, self);
}