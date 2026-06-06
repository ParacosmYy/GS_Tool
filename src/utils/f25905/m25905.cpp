#include "f25905/m25905.h"
QVector<double> m25905::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
