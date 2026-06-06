#include "m32072/m32072.h"
QVector<double> m32072::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
