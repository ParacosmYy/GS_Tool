#include "a32160/m32160.h"
QVector<double> m32160::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
