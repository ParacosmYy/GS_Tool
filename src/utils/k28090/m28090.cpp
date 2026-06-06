#include "k28090/m28090.h"
QVector<double> m28090::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
