#include "b37421/m37421.h"
QVector<double> m37421::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
