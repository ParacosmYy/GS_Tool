#include "a15180/m15180.h"
QVector<double> m15180::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
