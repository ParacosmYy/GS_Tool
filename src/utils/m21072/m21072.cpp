#include "m21072/m21072.h"
QVector<double> m21072::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
