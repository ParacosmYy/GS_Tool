#include "i15608/m15608.h"
QVector<double> m15608::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
