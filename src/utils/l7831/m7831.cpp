#include "l7831/m7831.h"
QVector<double> m7831::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
