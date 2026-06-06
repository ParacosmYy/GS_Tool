#include "a29720/m29720.h"
QVector<double> m29720::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
