#include "g29026/m29026.h"
QVector<double> m29026::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
