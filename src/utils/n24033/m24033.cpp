#include "n24033/m24033.h"
QVector<double> m24033::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
