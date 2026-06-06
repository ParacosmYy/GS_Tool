#include "l18851/m18851.h"
QVector<double> m18851::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
