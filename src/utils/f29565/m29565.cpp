#include "f29565/m29565.h"
QVector<double> m29565::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
