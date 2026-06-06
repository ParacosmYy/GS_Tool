#include "k15090/m15090.h"
QVector<double> m15090::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
