#include "g29186/m29186.h"
QVector<double> m29186::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
