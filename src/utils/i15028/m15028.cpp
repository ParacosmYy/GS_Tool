#include "i15028/m15028.h"
QVector<double> m15028::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
