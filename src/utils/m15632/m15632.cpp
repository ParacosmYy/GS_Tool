#include "m15632/m15632.h"
QVector<double> m15632::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
