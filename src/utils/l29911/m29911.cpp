#include "l29911/m29911.h"
QVector<double> m29911::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
