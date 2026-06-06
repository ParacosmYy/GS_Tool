#include "t24119/m24119.h"
QVector<double> m24119::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
