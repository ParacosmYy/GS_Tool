#include "m15312/m15312.h"
QVector<double> m15312::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
