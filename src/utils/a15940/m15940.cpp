#include "a15940/m15940.h"
QVector<double> m15940::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
