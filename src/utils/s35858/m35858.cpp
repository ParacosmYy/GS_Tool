#include "s35858/m35858.h"
QVector<double> m35858::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
