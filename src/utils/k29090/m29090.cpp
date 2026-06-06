#include "k29090/m29090.h"
QVector<double> m29090::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
