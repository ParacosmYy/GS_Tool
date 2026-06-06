#include "b15021/m15021.h"
QVector<double> m15021::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
