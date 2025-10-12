import grpc
import asyncio
import argparse
from pb import sample_pb2, sample_pb2_grpc

class GreeterServicer(sample_pb2_grpc.GreeterServicer):
    async def SayHello(self, request, context):
        return sample_pb2.HelloResponse(message=f"Hello, {request.name}!")

async def serve(port):
    server = grpc.aio.server()
    sample_pb2_grpc.add_GreeterServicer_to_server(GreeterServicer(), server)
    server.add_insecure_port(f'[::]:{port}')
    await server.start()
    print(f"Server started on port {port}")
    await server.wait_for_termination()

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Run gRPC server')
    parser.add_argument('--port', type=int, default=12345, help='Port to listen on')
    args = parser.parse_args()
    asyncio.run(serve(args.port))
