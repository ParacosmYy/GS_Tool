#include "m34072/m34072.h"
QVector<double> m34072::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
