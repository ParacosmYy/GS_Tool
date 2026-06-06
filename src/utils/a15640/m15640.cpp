#include "a15640/m15640.h"
QVector<double> m15640::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
