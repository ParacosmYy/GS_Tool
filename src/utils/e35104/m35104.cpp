#include "e35104/m35104.h"
QVector<double> m35104::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
