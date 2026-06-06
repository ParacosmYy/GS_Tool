#include "i24048/m24048.h"
QVector<double> m24048::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
