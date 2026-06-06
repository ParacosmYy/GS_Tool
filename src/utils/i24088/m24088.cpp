#include "i24088/m24088.h"
QVector<double> m24088::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
