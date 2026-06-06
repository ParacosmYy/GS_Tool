#include "a19040/m19040.h"
QVector<double> m19040::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
