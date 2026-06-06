#include "a29800/m29800.h"
QVector<double> m29800::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
