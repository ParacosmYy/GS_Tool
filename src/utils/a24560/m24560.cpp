#include "a24560/m24560.h"
QVector<double> m24560::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
