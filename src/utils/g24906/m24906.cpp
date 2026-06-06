#include "g24906/m24906.h"
QVector<double> m24906::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
