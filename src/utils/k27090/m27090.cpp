#include "k27090/m27090.h"
QVector<double> m27090::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
