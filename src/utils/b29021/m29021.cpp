#include "b29021/m29021.h"
QVector<double> m29021::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
