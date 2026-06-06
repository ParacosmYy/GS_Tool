#include "i27208/m27208.h"
QVector<double> m27208::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
