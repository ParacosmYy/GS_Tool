#include "m32092/m32092.h"
QVector<double> m32092::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
