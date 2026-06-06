#include "k19090/m19090.h"
QVector<double> m19090::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
