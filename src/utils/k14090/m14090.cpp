#include "k14090/m14090.h"
QVector<double> m14090::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
