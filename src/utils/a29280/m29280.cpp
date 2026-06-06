#include "a29280/m29280.h"
QVector<double> m29280::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
