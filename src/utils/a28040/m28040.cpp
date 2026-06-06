#include "a28040/m28040.h"
QVector<double> m28040::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
