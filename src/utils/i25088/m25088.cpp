#include "i25088/m25088.h"
QVector<double> m25088::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
