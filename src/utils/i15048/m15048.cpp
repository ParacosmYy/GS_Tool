#include "i15048/m15048.h"
QVector<double> m15048::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
