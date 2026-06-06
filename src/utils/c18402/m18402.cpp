#include "c18402/m18402.h"
QVector<double> m18402::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
