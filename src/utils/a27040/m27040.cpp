#include "a27040/m27040.h"
QVector<double> m27040::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
