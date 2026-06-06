#include "k29890/m29890.h"
QVector<double> m29890::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
