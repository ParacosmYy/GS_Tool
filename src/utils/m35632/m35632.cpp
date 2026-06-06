#include "m35632/m35632.h"
QVector<double> m35632::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
