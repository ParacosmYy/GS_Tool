#include "b18781/m18781.h"
QVector<double> m18781::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
