#include "i17048/m17048.h"
QVector<double> m17048::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
